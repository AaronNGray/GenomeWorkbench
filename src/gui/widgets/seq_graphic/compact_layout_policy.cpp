/*  $Id: compact_layout_policy.cpp 34827 2016-02-17 00:59:08Z rudnev $
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
#include <gui/widgets/seq_graphic/compact_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <corelib/ncbi_limits.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CCompactLayout
///////////////////////////////////////////////////////////////////////////////
typedef map<TModelUnit, TModelUnit> THeight;

static bool Height_Comparer(THeight::value_type &i1, THeight::value_type &i2)
{
    return i1.second < i2.second;
}

void CCompactLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    TModelUnit left = DBL_MAX;
    TModelUnit right = 0.0;

    THeight height;
    height[0.0] = 0.0;
    CLayoutGroup::TObjectList& objs(group.GetChildren());

    TObjectList objs_temp = objs;
    objs_temp.sort(SGlyphBySeqSize::s_CompareCRefs);

    while ( !objs_temp.empty() ) {
        TObjectList::iterator iter = objs_temp.begin();
        while (iter != objs_temp.end()) {
            CSeqGlyph* glyph = *iter;
            TModelRange range(glyph->GetLeft() - m_MinDist,
                glyph->GetRight() + m_MinDist);
            if (range.GetFrom() < 0.0) {
                range.SetFrom(0.0);
            }
            THeight::iterator f_iter = height.upper_bound(range.GetFrom());
            TModelUnit h = -1.0;
            TModelUnit upper = DBL_MAX;
            if (f_iter == height.end()) {
                h = height.rbegin()->second;
            } else if ((upper = f_iter->first) >= range.GetTo()) {
                h = (--f_iter)->second;
            }
            if (h >= 0.0) {
                left = min(left, range.GetFrom());
                right = max(right, range.GetTo());
                glyph->SetTop(h + m_VertSpace);
                height[range.GetFrom()] = h + m_VertSpace + glyph->GetHeight();
                if (upper > range.GetTo()) {
                    height[range.GetTo()] = h;
                }
                iter = objs_temp.erase(iter);
            } else {
                ++iter;
            }
        }

        THeight::iterator i_pre = height.begin();
        THeight::iterator i_curr = i_pre;
        THeight::iterator i_next = i_pre;
        ++i_next;
        while (i_next != height.end()) {
            while (i_next != height.end()  &&
                (i_curr->second > i_pre->second  ||
                i_curr->second > i_next->second)) {
                i_pre = i_curr;
                i_curr = i_next;
                ++i_next;
            }
            if (i_next == height.end()) {
                if (i_curr->second < i_pre->second) {
                    i_curr->second = i_pre->second;
                }
                break;
            }
            TModelUnit max = std::max<TModelUnit>(i_pre->second, i_next->second);
            TModelUnit min = std::min<TModelUnit>(i_pre->second, i_next->second);
            i_curr->second = i_curr->second == min ? max : min;
            while (i_next != height.end()  &&
                i_curr->second <= i_next->second) {
                i_pre = i_curr;
                i_curr = i_next;
                ++i_next;
            }
        }
        i_next = i_curr = height.begin();
        ++i_next;
        while (i_next != height.end()) {
            while (i_next != height.end()  &&  i_curr->second != i_next->second) {
                i_curr = i_next;
                ++i_next;
            }
            if (i_next != height.end()) {
                height.erase(i_next);
                i_next = i_curr;
                ++i_next;
            }
        }
    }

    bound.m_Height =
        std::max_element(height.begin(), height.end(), Height_Comparer)->second;
    bound.m_X = left;
    bound.m_Width = right - left;
}

END_NCBI_SCOPE
