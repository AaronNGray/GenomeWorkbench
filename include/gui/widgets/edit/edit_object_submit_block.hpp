#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT_SUBMIT_BLOCK__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT_SUBMIT_BLOCK__HPP

/*  $Id: edit_object_submit_block.hpp 39708 2017-10-27 14:03:13Z filippov $
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

#include <gui/widgets/edit/edit_object.hpp>

#include <gui/widgets/edit/submitblockpanel.hpp>
#include <gui/widgets/edit/contactpanel.hpp>
#include <gui/widgets/edit/author_affiliation_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>

class wxTreebook;
class wxPanel;


BEGIN_NCBI_SCOPE

class CSubmitRemarksPanel;

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjectSubmit_block : public CObject, public IEditObject
{
public:
    CEditObjectSubmit_block(const CObject& object,
                        objects::CScope& scope, bool createMode)
        : m_Object(&object), m_Scope(&scope), m_CreateMode(createMode), m_Window() {}

    virtual wxWindow* CreateWindow(wxWindow* parent);
    virtual IEditCommand* GetEditCommand();

private:
    enum {
        ID_NOTEBOOK = 10015
    };

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    bool m_CreateMode;

    CRef<CSerialObject> m_EditedSubmitBlock;
    CIRef<IEditCommand> m_EditAction;

    wxPanel* m_Window;
    wxTreebook* m_Treebook;
    CSubmitBlockPanel* m_SubmitBlockPanel;
    CContactPanel* m_ContactPanel;
    CAuthorNamesPanel* m_AuthorsPanel;
    CAuthorAffiliationPanel* m_AffilPanel;
    CSubmitRemarksPanel* m_Remarks;
    void x_TransferDataToWindow();
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___EDIT_OBJECT_SUBMIT_BLOCK__HPP
