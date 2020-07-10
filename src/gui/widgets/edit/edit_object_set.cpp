/*  $Id: edit_object_set.cpp 37188 2016-12-13 19:17:31Z asztalos $
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
 * Authors: Colleen Bollin, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/edit_object_set.hpp>
#include <gui/widgets/edit/set_class_panel.hpp>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>

#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/objutils/utils.hpp>

#ifdef _DEBUG
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#endif

#include <wx/toplevel.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


wxWindow* CEditObjectSet::CreateWindow(wxWindow* parent)
{
    if (m_Window) {
        NCBI_THROW(CException, eUnknown, 
                   "CEditObjectSet::CreateWindow - Window already created!");
    }
    wxTopLevelWindow* gui_widget = dynamic_cast<wxTopLevelWindow*>(parent);
    const CBioseq_set& set = dynamic_cast<const CBioseq_set&>(*m_Object);

    CSetClassPanel* pnl = new CSetClassPanel( parent );    
    pnl->SetClass(set.IsSetClass() ? set.GetClass() : CBioseq_set::eClass_not_set);
    m_Window = pnl;
    if (gui_widget)
        gui_widget->SetTitle(_("Bioseq-set"));
    return m_Window;
}


IEditCommand* CEditObjectSet::GetEditCommand()
{
    if (!m_Window)
        return 0;
    CSetClassPanel* pnl = dynamic_cast<CSetClassPanel*>(m_Window);
    if (!pnl) 
        return 0;
    
    if (!m_EditAction) {
        m_Window->TransferDataFromWindow();
        const CBioseq_set& set = dynamic_cast<const CBioseq_set&>(*m_Object);
        CBioseq_set_Handle bh = m_Scope->GetBioseq_setHandle(set);
        CRef<CBioseq_set> new_set(new CBioseq_set());
        new_set->Assign(set);
        new_set->SetClass(pnl->GetClass());
        if (!CBioseq_set::NeedsDocsumTitle(new_set->GetClass())) {
            if (new_set->IsSetDescr() && !new_set->GetDescr().Get().empty()) {
                auto it = new_set->SetDescr().Set().begin();
                while (it != new_set->SetDescr().Set().end()) {
                    if ((*it)->IsTitle()) {
                        it = new_set->SetDescr().Set().erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }
        }

        if (new_set->SetDescr().Set().empty()) {
            new_set->ResetDescr();
        }

        CCmdChangeBioseqSet* cmd = new CCmdChangeBioseqSet(bh, *new_set);
        m_EditAction.Reset(cmd);
    }

    return m_EditAction.GetPointer();

}


END_NCBI_SCOPE

