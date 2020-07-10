/*  $Id: cmd_cleanup.cpp 41007 2018-05-09 18:36:32Z katargir $
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


#include <ncbi_pch.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>

#include "cmd_cleanup.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CIRef<IEditCommand> CCmdCleanup::x_CreateActionCmd()
{
    CConstRef<CSeq_entry> entry = m_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults(); 
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);

    if (m_DoTax) {
        CRef<CCmdComposite> tax = TaxonomyLookupCommand(new_seh);
        if (tax) {
            tax->Execute();
        }
    }

    CCleanup cleanup;
    cleanup.SetScope(&scope2);
        
    // perform BasicCleanup and ExtendedCleanup
    try {
        CConstRef<CCleanupChange> changes;        
        if (m_Extended) {
            // basic cleanup is intrinsically part of ExtendedCleanup
            changes = cleanup.ExtendedCleanup(new_seh, CCleanup::eClean_SyncGenCodes|CCleanup::eClean_KeepTopSet);
        } else {
            changes = cleanup.BasicCleanup(const_cast<CSeq_entry&>(*copy), CCleanup::eClean_SyncGenCodes);
        }
    } catch (CException& e) {
        LOG_POST(Error << "error in cleanup: " << e.GetMsg());
    }

    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(m_seh, copy));
    return CIRef<IEditCommand>(clean);
}

string CCmdCleanup::GetLabel()
{
    return "Cleanup";
}

END_NCBI_SCOPE
