#ifndef GUI_WIDGETS_OBJECT_LIST___OBJECT_TEXT_ITEM__HPP
#define GUI_WIDGETS_OBJECT_LIST___OBJECT_TEXT_ITEM__HPP

/*  $Id: object_text_item.hpp 32647 2015-04-07 16:12:16Z katargir $
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

#include <gui/gui_export.h>
#include <corelib/ncbiobj.hpp>

#include <gui/widgets/text_widget/text_select_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT CObjectTextItem : public CTextSelectItem
{
public:
    enum EMode {
        kBrief,
        kFull
    };

    static ITextItem* CreateTextItem(SConstScopedObject& object, EMode mode, ICanceled* cancel);

    virtual const CConstRef<CObject> GetAssosiatedObject() const { return m_Object; }
    virtual objects::CScope*         GetAssosiatedScope() const { return m_Scope.GetPointer(); }

protected:
    CObjectTextItem(SConstScopedObject& object, EMode mode)
        : m_Object(object.object), m_Scope(object.scope), m_Mode(mode) {}

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    EMode m_Mode;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_OBJECT_LIST___OBJECT_TEXT_ITEM__HPP
