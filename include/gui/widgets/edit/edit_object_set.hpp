#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT_SET__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT_SET__HPP

/*  $Id: edit_object_set.hpp 31581 2014-10-24 17:09:27Z bollin $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <gui/gui_export.h>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/desc_placement_panel.hpp>

class wxPanel;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjectSet : public CObject, public IEditObject
{
public:
    CEditObjectSet(const CObject& object,
                        objects::CScope& scope)
        : m_Object(&object), m_Scope(&scope), m_Window(NULL) {}

    virtual wxWindow* CreateWindow(wxWindow* parent);
    virtual IEditCommand* GetEditCommand();

private:
    enum {
        ID_NOTEBOOK = 10015
    };

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    CIRef<IEditCommand> m_EditAction;

    wxPanel* m_Window;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___EDIT_OBJECT_SET__HPP
