#ifndef GUI_WIDGETS_TEXT_WIDGET___ROOT_TEXT_ITEM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___ROOT_TEXT_ITEM__HPP

/*  $Id: root_text_item.hpp 28692 2013-08-19 16:38:23Z katargir $
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

#include <gui/widgets/text_widget/composite_text_item.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CRootTextItem : public CCompositeTextItem
{
public:
    CRootTextItem() {}
    ~CRootTextItem();

    void AddUserObject(const string& name, wxObject* obj);
    wxObject* GetUserObject(const string& name);

    typedef map<string, wxObject*> TContainer;

protected:
    TContainer m_UserObjects;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___ROOT_TEXT_ITEM__HPP
