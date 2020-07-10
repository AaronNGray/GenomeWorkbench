/*  $Id: propagate_descriptors.cpp 43899 2019-09-17 12:09:19Z choi $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/edit/propagate_descriptors.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

CRef<CCmdComposite> CPropagateDescriptors::GetPropagateDownCommand(CSeq_entry_Handle seh)
{
    if (!seh.IsSet())
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> cmd(new CCmdComposite("Propagate Descriptors"));
    
    CBioseq_set_Handle bioseq_set_h = seh.GetSet();
    if (bioseq_set_h.IsSetClass() && bioseq_set_h.GetClass() == CBioseq_set::eClass_genbank && 
        bioseq_set_h.GetCompleteBioseq_set()->IsSetSeq_set() && ! bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().empty() &&
        bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().front()->IsSet())
        bioseq_set_h = seh.GetScope().GetBioseq_setHandle(bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().front()->GetSet());

    
    vector<CRef<CSeqdesc> > descs;
    CSeq_entry_Handle pseh = bioseq_set_h.GetParentEntry();    
    for( CSeqdesc_CI desc_ci( pseh, CSeqdesc::e_not_set, 1); desc_ci; ++desc_ci ) 
    {
        if (!ShouldStayInPlace(*desc_ci))
        {
            descs.push_back(CRef<CSeqdesc>( SerialClone(*desc_ci) ) );
            cmd->AddCommand( *CRef<CCmdDelDesc>(new CCmdDelDesc(pseh, *desc_ci)));
        }
    }
    
    
    // copy to all immediate children  
    for(CSeq_entry_CI direct_child_ci( bioseq_set_h, CSeq_entry_CI::eNonRecursive ); direct_child_ci; ++direct_child_ci ) 
    {
        CSeq_entry_Handle cseh = *direct_child_ci;
        for (size_t i = 0; i < descs.size(); i++)
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(*descs[i]);
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(cseh, *new_desc)));
        }
    }    
    return cmd;
}

CRef<CCmdComposite> CPropagateDescriptors::GetPropagateUpCommand(CSeq_entry_Handle seh)
{
    if (!seh.IsSet())
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> cmd(new CCmdComposite("Propagate Descriptors"));
    
    CBioseq_set_Handle bioseq_set_h = seh.GetSet();
    if (bioseq_set_h.IsSetClass() && bioseq_set_h.GetClass() == CBioseq_set::eClass_genbank && 
        bioseq_set_h.GetCompleteBioseq_set()->IsSetSeq_set() && ! bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().empty() &&
        bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().front()->IsSet())
        bioseq_set_h = seh.GetScope().GetBioseq_setHandle(bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().front()->GetSet());

    
    vector<const CSeqdesc*> descs;
   // copy from all immediate children
    CSeq_entry_CI direct_child_ci( bioseq_set_h, CSeq_entry_CI::eNonRecursive );
    {
	CSeq_entry_Handle cseh = *direct_child_ci;	
	for( CSeqdesc_CI desc_ci( cseh, CSeqdesc::e_not_set, 1); desc_ci; ++desc_ci ) 
	{
            if (!ShouldStayInPlace(*desc_ci))
            {
                descs.push_back(&*desc_ci);
            }
	}
	++direct_child_ci;
    }
    set<size_t> to_delete;
    for( ; direct_child_ci; ++direct_child_ci ) 
    {
	CSeq_entry_Handle cseh = *direct_child_ci;
	for (size_t i = 0; i < descs.size(); i++)
	{
	    bool found = false;
	    for( CSeqdesc_CI desc_ci( cseh, CSeqdesc::e_not_set, 1); desc_ci; ++desc_ci ) 
	    {
		if (desc_ci->Equals(*descs[i]))
		{
		    found = true;
		}
	    } 
	    if (!found)
		to_delete.insert(i);
	}
    }    
    for (set<size_t>::reverse_iterator i = to_delete.rbegin(); i != to_delete.rend(); ++i)
    {
	descs.erase(descs.begin() + (*i));
    }
    
    for( CSeq_entry_CI direct_child_ci( bioseq_set_h, CSeq_entry_CI::eNonRecursive ); direct_child_ci; ++direct_child_ci ) 
    {
	CSeq_entry_Handle cseh = *direct_child_ci;
	for (size_t i = 0; i < descs.size(); i++)
	{
	    for( CSeqdesc_CI desc_ci( cseh, CSeqdesc::e_not_set, 1); desc_ci; ++desc_ci ) 
	    {
		if (desc_ci->Equals(*descs[i]))
		{
		    cmd->AddCommand( *CRef<CCmdDelDesc>(new CCmdDelDesc(cseh, *desc_ci)));
		}
	    } 
	}
    }    


    CSeq_entry_Handle pseh = bioseq_set_h.GetParentEntry();
    for (size_t i = 0; i < descs.size(); i++)
    {
	CRef<CSeqdesc> new_desc(new CSeqdesc);
	new_desc->Assign(*descs[i]);
	cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(pseh, *new_desc)));
    }

    return cmd;
}

bool CPropagateDescriptors::ShouldStayInPlace(const CSeqdesc& desc)
{
    bool ret = desc.IsTitle(); 
    ret |= desc.IsMolinfo();
    ret |= desc.IsSource();
    ret |= desc.IsCreate_date();
    ret |= desc.IsUpdate_date();
    ret |= desc.IsUser() && desc.GetUser().GetObjectType() == CUser_object::eObjectType_DBLink;
	ret |= desc.IsUser() && desc.GetUser().IsSetType() && desc.GetUser().GetType().IsStr() && desc.GetUser().GetType().GetStr() == "GbenchModification";

    return ret;
}

END_NCBI_SCOPE
