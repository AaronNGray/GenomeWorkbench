/*  $Id: features_group.cpp 29744 2014-02-05 20:42:29Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>

#include "features_group.hpp"

BEGIN_NCBI_SCOPE

CFeaturesGroup::CFeaturesGroup(CTextItem* collapsedItem, CTextItem* expandedItem,
                               bool expand)
    : CExpandItem(collapsedItem, expandedItem, expand)
{
    SetSelectable(false);
}

void CFeaturesGroup::ExpandAll(CTextPanelContext* context)
{
    if (!this->IsExpanded())
        this->Toggle(context, true);

    CCompositeTextItem* composite = (CCompositeTextItem*)m_Items[1];
    for (size_t i = 0; i < composite->GetItemCount(); ++i) {
        IExpandable* expandable = dynamic_cast<IExpandable*>(composite->GetItem(i));
        if (expandable != 0 && !expandable->IsExpanded())
            expandable->Toggle(context, true);
    }
}

void CFeaturesGroup::CollapseAll(CTextPanelContext* context)
{
    if (this->IsExpanded())
        this->Toggle(context, true);

    CCompositeTextItem* composite = (CCompositeTextItem*)m_Items[1];
    for (size_t i = 0; i < composite->GetItemCount(); ++i) {
        IExpandable* expandable = dynamic_cast<IExpandable*>(composite->GetItem(i));
        if (expandable != 0 && expandable->IsExpanded())
            expandable->Toggle(context, true);
    }
}

END_NCBI_SCOPE
