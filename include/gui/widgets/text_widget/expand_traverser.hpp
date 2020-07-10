#ifndef GUI_WIDGETS_TEXT_WIDGET___EXPAND_TRAVERSER__HPP
#define GUI_WIDGETS_TEXT_WIDGET___EXPAND_TRAVERSER__HPP

/*  $Id: expand_traverser.hpp 39188 2017-08-16 20:50:20Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/text_widget/composite_traverser.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CExpandTraverser : public ICompositeTraverser
{
public:
    CExpandTraverser(CTextPanelContext& context, bool expand) : m_Context(context), m_Expand(expand) {}

    virtual bool ProcessItem(CTextItem& /*textItem*/) { return true; }
    virtual bool ProcessItem(CExpandItem& expandItem)
    {
        if ((m_Expand && !expandItem.IsExpanded()) || (!m_Expand && expandItem.IsExpanded())) {
            expandItem.Toggle(&m_Context, false);
        }
        return true;
    }
    CTextPanelContext& m_Context;
    bool m_Expand;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___EXPAND_TRAVERSER__HPP
