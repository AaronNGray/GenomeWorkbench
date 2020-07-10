/*  $Id: column_layout_policy.cpp 39020 2017-07-20 15:44:33Z shkeda $
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
#include <corelib/ncbiutil.hpp>
#include <gui/widgets/seq_graphic/column_layout_policy.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <corelib/ncbi_limits.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

static const int kVertScreenPixel = 400;
static const size_t kMinRowPerGroup = 20;

///////////////////////////////////////////////////////////////////////////////
///   CColumnLayout
///////////////////////////////////////////////////////////////////////////////

void CColumnLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    typedef vector<TModelUnit> TRows;
    TRows   rows;
    TLayout layout;
    auto tearline = group.GetTearline();
    if (tearline > 0)
        layout.reserve(tearline);

    CLayoutGroup::TObjectList& objs(group.GetChildren());

    objs.sort(SGlyphBySeqPos::s_CompareCRefs);
    NON_CONST_ITERATE (TObjectList, iter, objs) {
        CSeqGlyph* glyph = *iter;
        glyph->SetTearline(tearline);
        glyph->SetVisible(true);
        TModelRange range(glyph->GetLeft() - m_MinDist, glyph->GetRight());
        size_t curr = 0;
        for (; curr < rows.size(); ++curr) {
            if (rows[curr] < range.GetFrom()) {
                break;
            }
        }

        if (curr < rows.size()) {
            layout[curr].push_back(*iter);
            (*iter)->SetRowNum(curr);
            if (curr > 0  &&  range.GetTo() < rows[curr-1]  &&
                range.GetLength() * 2.0 > rows[curr-1] - range.GetFrom()) {
                rows[curr] = rows[curr-1];
            } else {
                rows[curr] = range.GetTo();
            }
        } else {
            if (tearline > 0 && (layout.size() > tearline)) {
                (*iter)->SetVisible(false);
            } else {
                rows.push_back(range.GetTo());
                (*iter)->SetRowNum(layout.size());
                TLayoutRow row;
                row.push_back(*iter);
                layout.push_back(row);
            }
        }
    }

    TModelUnit obj_h = 1.0;
    if ( !objs.empty() ) {
        obj_h = objs.front()->GetHeight();
    }
    size_t row_num = kVertScreenPixel / (obj_h + m_VertSpace);
    row_num = max(kMinRowPerGroup, row_num);
    if (layout.size() > row_num   &&  m_LimitRowPerGroup) {
        x_SeparateObjects(group, layout, bound, row_num, m_VertSpace);
    } else {
        x_SetObjectPos(layout, bound);
    }
}


void CColumnLayout::x_SetObjectPos(TLayout& layout, SBoundingBox& bound) const
{
    bound.m_Height = 0.0;
    bound.m_Width = 0.0;
    bound.m_X = DBL_MAX;
    TModelUnit max_h = 0.0;
    TModelUnit right = 0.0;
    size_t current_row = 0;
    NON_CONST_ITERATE (TLayout, r_iter, layout) {

        bool row_is_visible = true;

        NON_CONST_ITERATE(TLayoutRow, iter, *r_iter) {
            if ((*iter)->GetTearline() > 0 && (current_row > (*iter)->GetTearline())) {
                row_is_visible = false;
                break;
            }
        }

        ++current_row;
        if (!row_is_visible)
            continue;

        bound.m_Height += m_VertSpace;
        NON_CONST_ITERATE (TLayoutRow, iter, *r_iter) {
            (*iter)->SetTop(bound.m_Height);
            max_h = max(max_h, (*iter)->GetHeight());
            bound.m_X = min(bound.m_X, (*iter)->GetLeft());
            right = max(right, (*iter)->GetRight());
        }
        bound.m_Height += max_h;
        max_h = 0.0;
    }
    bound.m_Width = right - bound.m_X;
}


END_NCBI_SCOPE
