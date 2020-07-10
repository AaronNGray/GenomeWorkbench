/* $Id: remove_set.cpp 32893 2015-05-04 16:30:38Z filippov $
 * ===========================================================================
 *
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
 * Authors:  Igor Filippov
 *  
 */

#include <ncbi_pch.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/remove_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

void CRemoveSet::MoveDescrToEntries(CBioseq_set& se)
{    
    EDIT_EACH_SEQENTRY_ON_SEQSET(it, se)
    {
        if (se.IsSetDescr())
        {
            for (CSeq_descr::Tdata::const_iterator desc = se.GetDescr().Get().begin(); desc != se.GetDescr().Get().end(); ++desc)
            {
                if (!(*desc)->IsTitle())
                    (*it)->SetDescr().Set().push_back(*desc);            
            }
        }              
    }       

    se.ResetDescr();
}

TConstScopedObjects CRemoveSet::GetSelectedObjects(IWorkbench* wb)
{
    TConstScopedObjects sel_objects;
    if (!wb) {
        return sel_objects;
    }

    CIRef<CSelectionService> sel_srv = wb->GetServiceByType<CSelectionService>();
    if (sel_srv)
    {
        sel_srv->GetActiveObjects(sel_objects);
    }
   if (sel_objects.empty())
    {
        GetViewObjects(wb, sel_objects);
    }   
    return sel_objects;
}

ICommandProccessor* CRemoveSet::GetCmdProcessor(IWorkbench* wb, CScope& scope)
{
    CIRef<CProjectService> srv = wb->GetServiceByType<CProjectService>();
    if (!srv)
        return NULL;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return NULL;
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(scope));
    if (!doc)
        return NULL;
    return &doc->GetUndoManager(); 
}

void CRemoveSet::RemoveOneSet(const CBioseq_set* bioseqset, CScope& scope, CRef<CBioseq_set> &new_set)
{  
    if (!bioseqset)
        return;
    
    if (!bioseqset->IsSetClass())
        return;
    
    if (bioseqset->GetClass() == CBioseq_set::eClass_nuc_prot || bioseqset->GetClass() ==  CBioseq_set::eClass_segset 
        ||  bioseqset->GetClass() ==  CBioseq_set::eClass_parts || bioseqset->GetClass() ==  CBioseq_set::eClass_not_set)
        return;            

    CBioseq_set* se = NULL;
    CBioseq_set::TSeq_set::iterator remove_me;
    EDIT_EACH_SEQENTRY_ON_SEQSET(it, *new_set)
    {
        if ((*it)->IsSet() && bioseqset->Equals((*it)->GetSet()))
        {
            se = &(*it)->SetSet();
            remove_me = it;
            break;
        }
    }
    if (!se)
        return;
    MoveDescrToEntries(*se);
    EDIT_EACH_SEQENTRY_ON_SEQSET(it, *se)
    {
        new_set->SetSeq_set().push_back(*it);
    }
    new_set->SetSeq_set().erase(remove_me);
}

void CRemoveSet::remove_set(IWorkbench* wb)
{
    const CBioseq_set* bioseqset = NULL;
    TConstScopedObjects objects = GetSelectedObjects(wb);
    if (objects.empty())
        return;
    CScope &scope =  *objects.front().scope;

    ICommandProccessor* cmdProcessor = GetCmdProcessor(wb,scope);   
    if (!cmdProcessor)
        return;

    ITERATE (TConstScopedObjects, it, objects) 
    {
        bioseqset = dynamic_cast<const objects::CBioseq_set*>((*it).object.GetPointer());
        if (bioseqset) 
        {
            break;
        }
    }

    if (!bioseqset)
        return;

    CBioseq_set_Handle bssh = scope.GetBioseq_setHandle(*bioseqset);
    CBioseq_set_Handle parent =  bssh.GetParentBioseq_set();
    if (!parent)
        return;

    CRef<CBioseq_set> new_set(new CBioseq_set());
    new_set->Assign(*parent.GetCompleteBioseq_set());
   
    RemoveOneSet(bioseqset, scope, new_set);
    CRef<CCmdChangeBioseqSet> cmd(new CCmdChangeBioseqSet(parent, *new_set));

    cmdProcessor->Execute(cmd);
}

void CRemoveSet::remove_sets_from_set(IWorkbench* wb)
{

    const CBioseq_set* bioseqset = NULL;
    TConstScopedObjects objects = GetSelectedObjects(wb);

    if (objects.empty())
        return;
    CScope &scope =  *objects.front().scope;

    ICommandProccessor* cmdProcessor = GetCmdProcessor(wb,scope);   
    if (!cmdProcessor)
        return;

    ITERATE (TConstScopedObjects, it, objects) 
    {
        bioseqset = dynamic_cast<const objects::CBioseq_set*>((*it).object.GetPointer());
        if (bioseqset) 
        {
            break;
        }
    }

    if (!bioseqset)
    {
        NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
        {
            CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
            if (seh) 
            {
                if (seh.IsSet())
                {
                    bioseqset = seh.GetSet().GetBioseq_setCore().GetPointer();
                    break;
                }
            }            
        }
    }
    if (!bioseqset)
        return;

    if (!bioseqset->IsSetClass())
        return;

    if (bioseqset->GetClass() ==  CBioseq_set::eClass_segset ||  bioseqset->GetClass() ==  CBioseq_set::eClass_parts || bioseqset->GetClass() ==  CBioseq_set::eClass_not_set)
        return;

    CBioseq_set_Handle parent = scope.GetBioseq_setHandle(*bioseqset);
    if (!parent)
        return;

    CRef<CBioseq_set> new_set(new CBioseq_set());
    new_set->Assign(*parent.GetCompleteBioseq_set());

    FOR_EACH_SEQENTRY_ON_SEQSET(it, *bioseqset)
    {
        if ((*it)->IsSet())
        {                   
            RemoveOneSet(&(*it)->GetSet(), scope, new_set);
        }
    }
 
    CRef<CCmdChangeBioseqSet> cmd(new CCmdChangeBioseqSet(parent, *new_set));
    cmdProcessor->Execute(cmd);
}

END_NCBI_SCOPE
