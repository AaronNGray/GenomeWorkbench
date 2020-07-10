/*  $Id: layout_policy.cpp 40918 2018-04-30 18:19:04Z shkeda $
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
#include <gui/widgets/seq_graphic/layout_policy.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///   CLayoutPolicy
///////////////////////////////////////////////////////////////////////////////

void ILayoutPolicy::x_SeparateObjects(CLayoutGroup& orig_group,
                                     TLayout& layout,
                                     SBoundingBox& bound,
                                     size_t group_size, int vert_space) const
{
    
    _ASSERT(group_size > 0);
    // create sub-groups
    typedef vector<TLayoutRow*> TLayoutRows;
    typedef vector<TLayoutRows> TLayouts;
    CLayoutGroup::TObjectList& objs(orig_group.GetChildren());

    TLayouts layouts;
    TLayoutRows layout_rows;
    layout_rows.reserve(group_size);
    size_t curr_row = 0;
    NON_CONST_ITERATE (TLayout, iter, layout) {
        if (curr_row >= group_size) {
            layouts.push_back(layout_rows);
            layout_rows.clear();
            curr_row = 0;
        } else {
            ++curr_row;
        }
        layout_rows.push_back(&*iter);
    }

    if (!layout_rows.empty()) {
        layouts.push_back(layout_rows);
    }

    if (layouts.size() == 1) {
        x_SetObjectPos(layout, bound);
        return;
    }

    objs.clear();

    CRef<CBoundaryParams> params(
        new CBoundaryParams(false, false, CRgbaColor(0.3f, 0.3f, 1.0f),
        CRgbaColor(0.5f, 0.5f, 1.0f, 0.4f), 2.0, 0));
    // showing the group boundary for debuging
    // params->SetShowBoundary(true);

    NON_CONST_ITERATE (TLayouts, iter, layouts) {
        CLayoutGroup* group = new CLayoutGroup(orig_group.GetLinkedFeat());
        if(orig_group.GetLevel() != -1) {
            group->SetLevel(orig_group.GetLevel()+1);
        }
        objs.push_back(CRef<CSeqGlyph>(group));
        group->SetLayoutPolicy(const_cast<ILayoutPolicy*>(this));

        TModelUnit height = 0.0;
        TModelUnit max_h = 0.0;
        TModelUnit left = DBL_MAX;
        TModelUnit right = 0.0;
        for (auto& row : *iter) {
            height += vert_space;
            for (auto& obj : *row) {
                group->PushBack(obj);
                obj->SetTop(height);
                max_h = max(max_h, obj->GetHeight());
                left = min(left, obj->GetLeft());
                right = max(right, obj->GetRight());
            }
            height += max_h;
            max_h = 0.0;
        }
        group->SetHeight(height);
        group->SetLeft(left);
        group->SetWidth(right - left);
        group->SetConfig(params);
    }
}


END_NCBI_SCOPE
