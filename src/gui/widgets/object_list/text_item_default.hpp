#ifndef GUI_WIDGETS_OBJECT_LIST___TEXT_ITEM_DEFAULT__HPP
#define GUI_WIDGETS_OBJECT_LIST___TEXT_ITEM_DEFAULT__HPP

/*  $Id: text_item_default.hpp 32647 2015-04-07 16:12:16Z katargir $
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

#include <gui/widgets/object_list/object_text_item.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>

BEGIN_NCBI_SCOPE

class CTextItemDefault : public CObjectTextItem
{
public:
    static ITextItem* CreateObject(SConstScopedObject& object, EMode mode);
    virtual void x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

private:
    CTextItemDefault(SConstScopedObject& object, EMode mode)
        : CObjectTextItem(object, mode) {}
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_OBJECT_LIST___TEXT_ITEM_DEFAULT__HPP
