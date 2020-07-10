/*  $Id: layered_layout_policy.cpp 40918 2018-04-30 18:19:04Z shkeda $
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
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>

BEGIN_NCBI_SCOPE

static const size_t kMaxObjectNum = 1000;
static const int kVertScreenPixel = 400;
static const size_t kMinRowPerGroup = 20;

///////////////////////////////////////////////////////////////////////////////
///   CLayeredLayout
///////////////////////////////////////////////////////////////////////////////

/// special sort by glyph range.
/// abosolute less comparison, no overlapping allowed
struct SGlyphRangeSorter
{
    bool operator()(const CRef<CSeqGlyph>& obj1,
                    const CRef<CSeqGlyph>& obj2) const
    {
        //return (obj1->GetRange().GetToOpen() < obj2->GetRange().GetFrom());
        return (obj1->GetRange() < obj2->GetRange()  &&
                obj1->GetRange().GetTo() < obj2->GetRange().GetFrom());
    }
};

/// special sort by glyph geometry size.
/// abosolute less comparison, no overlapping allowed
struct SGlyphSorter
{
    bool operator()(const CRef<CSeqGlyph>& obj1,
                    const CRef<CSeqGlyph>& obj2) const
    {
        return obj1->GetRight() < obj2->GetLeft();
    }
};


/// a dummy concreate glyph class for layout purpose.
class CDummyGlyph : public CSeqGlyph
{
public:
    CDummyGlyph()
    {
    }

    void SetRange(const TSeqRange& range)
    {
        m_Range = range;
    }

    void SetSize(TModelRange r)
    {
        SetLeft(r.GetFrom());
        SetWidth(r.GetLength());
    }

    /// access the position of this object.
    TSeqRange GetRange(void) const
    {
        return m_Range;
    }

protected:
    virtual void x_Draw() const {};
    virtual void x_UpdateBoundingBox() {};

private:
    TSeqRange m_Range;
};


void CLayeredLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    CLayoutGroup::TObjectList& objs(group.GetChildren());
    size_t obj_num = objs.size();
    // By default, the objects are sorted by size to promote larger-size
    // objects to the top.  Layout algorithm sorted by object size is
    // more expensive than the one sorted by object position. When the
    // object number reaches a predefined limit, we swith the layout
    // algorithm to the cheaper one.
    // Note: when there is a maximal row/height constraint, we relax the
    // maximal object limit (double it) to reduce the chance of overlapping
    // the larger objects into the last row.
    if (obj_num < kMaxObjectNum  ||
        (m_MaxRow > 0  &&  obj_num < kMaxObjectNum * 2)) {
        x_LayerBySize(group, bound);
    } else {
        x_LayerByPos(group, bound);
    }
}


void CLayeredLayout::x_LayerBySize(CLayoutGroup& group, SBoundingBox& bound) const
{

    TLayout layout;

    // If there is a constraint on the maximum row/height of layout,
    // we simply squeeze the unfit objects to the last row.
    TLayoutRow last_row;

    CLayoutGroup::TObjectList& objs(group.GetChildren());
    if ( !m_Sorted ) {
        objs.sort(SGlyphBySeqSize::s_CompareCRefs);
    }

    SGlyphSorter func;

    CDummyGlyph* range_holder = new CDummyGlyph;
    CRef<CSeqGlyph> ref_obj(range_holder);

    TObjectList::const_iterator iter = objs.begin();
    TObjectList::const_iterator end (objs.end());
    for ( ;  iter != end;  ++iter) {
        CRef<CSeqGlyph> ref(*iter);
        TModelRange intersect_range(ref->GetLeft() - m_MinDist,
            ref->GetRight() + m_MinDist);
        range_holder->SetSize(intersect_range);

        bool inserted = false;
        NON_CONST_ITERATE (TLayout, l_iter, layout) {
            bool intersects = false;

            TLayoutRow::iterator row_iter =
                std::lower_bound(l_iter->begin(), l_iter->end(), ref_obj, func); /* NCBI_FAKE_WARNING: WorkShop */

            if (row_iter != l_iter->end()) {
                TModelRange total_range(
                    (*row_iter)->GetLeft(), (*row_iter)->GetRight());;
                intersects =
                    intersect_range.IntersectingWith(total_range);
            }
            if ( !intersects ) {
                inserted = true;
                l_iter->insert(row_iter, ref);
                ref->SetRowNum(layout.size());
                break;
            }
        }
        if ( !inserted ) {
            if (m_MaxRow < 1  ||  (int)(layout.size()) < m_MaxRow - 1) {
                TLayoutRow row;
                row.push_back(ref);
                layout.push_back(row);
                ref->SetRowNum(layout.size());
            } else {
                last_row.push_back(ref);
            }
        }
    }

    if ( !last_row.empty()) {
        layout.push_back(last_row);
        auto row_num = layout.size();
        for (auto& o : last_row) {
            o->SetRowNum(row_num);
        }
    }

    TModelUnit obj_h = 1.0;
    if ( !objs.empty() ) {
        obj_h = objs.front()->GetHeight();
    }
    size_t row_num = kVertScreenPixel / (obj_h + m_VertSpace);
    row_num = max(kMinRowPerGroup, row_num);
    if (layout.size() > row_num  &&  m_LimitRowPerGroup  &&  m_MaxRow < 1) {
        x_SeparateObjects(group, layout, bound, row_num, m_VertSpace);
    } else {
        x_SetObjectPos(layout, bound);
    }
}


void CLayeredLayout::x_LayerByPos(CLayoutGroup& group, SBoundingBox& bound) const
{
    typedef vector<TModelUnit> TRows;

    TRows   rows;
    TLayout layout;

    // for cases where there is a max row number limit,
    // we want to pack all unfit objects to the last row.
    TLayoutRow last_row;
    CLayoutGroup::TObjectList& objs(group.GetChildren());

    if ( !m_Sorted ) {
        objs.sort(SGlyphBySeqPos::s_CompareCRefs);
    }
    NON_CONST_ITERATE (TObjectList, iter, objs) {
        CSeqGlyph* glyph = *iter;
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
            rows[curr] = range.GetTo();
        } else {
            if (m_MaxRow < 1  ||  (int)(layout.size()) < m_MaxRow - 1) {
                rows.push_back(range.GetTo());
                TLayoutRow row;
                row.push_back(*iter);
                layout.push_back(row);
                (*iter)->SetRowNum(layout.size());

            } else {
                last_row.push_back(*iter);
            }
        }
    }

    if ( !last_row.empty()) {
        layout.push_back(last_row);
        auto row_num = layout.size();
        for (auto& o : last_row) {
            o->SetRowNum(row_num);
        }
    }

    TModelUnit obj_h = 1.0;
    if ( !objs.empty() ) {
        obj_h = objs.front()->GetHeight();
    }
    size_t row_num = kVertScreenPixel / (obj_h + m_VertSpace);
    row_num = max(kMinRowPerGroup, row_num);
    if (layout.size() > row_num  &&  m_LimitRowPerGroup  &&  m_MaxRow < 1) {
        x_SeparateObjects(group, layout, bound, row_num, m_VertSpace);
    } else {
        x_SetObjectPos(layout, bound);
    }
}


void CLayeredLayout::x_SetObjectPos(TLayout& layout, SBoundingBox& bound) const
{
    bound.m_Height = 0.0;
    bound.m_Width = 0.0;
    bound.m_X = DBL_MAX;
    TModelUnit max_h = 0.0;
    TModelUnit right = 0.0;
    int total_row = layout.size();
    for (int row = 0; row < total_row; ++row) {
        TLayoutRow& curr_row = layout[row];
        bound.m_Height += m_VertSpace;
        // if the last row contains packed inlined objects with side label,
        // we need to make sure only labels that can fit will be shown.
        if (row == total_row - 1  &&  m_MaxRow > 0  &&
            m_MaxRow == total_row  && curr_row.front()->HasSideLabel()) {
            CInlineLayout in_layout;
            in_layout.SetTopMargin(0);
            TObjectList objs;
            SBoundingBox r_bound;
            std::copy(curr_row.begin(), curr_row.end(), std::back_inserter(objs));
            in_layout.BuildLayout(objs, r_bound);

            max_h = r_bound.m_Height;
            bound.m_X = min(bound.m_X, r_bound.m_X);
            right = max(right, r_bound.m_X + r_bound.m_Width);
            NON_CONST_ITERATE (TLayoutRow, iter, curr_row) {
                (*iter)->SetTop(bound.m_Height);
            }
        } else {
            NON_CONST_ITERATE (TLayoutRow, iter, curr_row) {
                (*iter)->SetTop(bound.m_Height);
                max_h = max(max_h, (*iter)->GetHeight());
                bound.m_X = min(bound.m_X, (*iter)->GetLeft());
                right = max(right, (*iter)->GetRight());
            }
        }
        bound.m_Height += max_h;
        max_h = 0.0;
    }
    bound.m_Width = right - bound.m_X;
}


END_NCBI_SCOPE
