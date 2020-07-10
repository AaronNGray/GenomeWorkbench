/*  $Id: cmd_del_bioseq.cpp 43490 2019-07-17 15:21:58Z filippov $
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
 * Authors:  Roman Katargin, Igor Filippov
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/cmd_del_bioseq.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/objmgr_exception.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdDelBioseqInst::CCmdDelBioseqInst(CBioseq_Handle& bsh)
    : m_BSH(bsh)
{
}

void CCmdDelBioseqInst::Execute()  
{
    if (m_BSH.IsRemoved()) return;
    objects::CBioseq_set_Handle bssh = m_BSH.GetParentBioseq_set(); // get the parent set - could be empty
    objects::CSeq_entry_Handle seh =  m_BSH.GetSeq_entry_Handle();
    if (bssh) {
        m_index = bssh.GetSeq_entry_Index(seh);   // undocumented function but seems to do what it says
        m_Set_SEH = bssh.GetParentEntry();
    } else {
        m_index = -1;
        m_Set_SEH = seh;
    }

    m_Inst.Reset();
    m_Inst = m_BSH.GetCompleteBioseq();
    bool is_prot = m_BSH.IsAa();
    CBioseq_EditHandle eh(m_BSH);
    eh.Remove();
    if (is_prot && bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot
        && !bssh.IsEmptySeq_set() && bssh.GetBioseq_setCore()->GetSeq_set().size() == 1) {

        CSeq_entry_Handle subentry;
        try
        {
            subentry = m_Set_SEH.GetSingleSubEntry();
        } catch (const CObjMgrException&) {}
        if (subentry && subentry.IsSeq())  {
            // Before renormalizing, SAVE setlevel annots so that you can reattach 
            // them to the set during UNDO
            CSeq_annot_CI annot_ci( m_Set_SEH, 
                                    CSeq_annot_CI::eSearch_entry /*nonrecursive*/ );
            m_SetLevelAnnots.clear();
            for( ; annot_ci; ++annot_ci ) {
                m_SetLevelAnnots.push_back( *annot_ci );
            }

            // Before renormalizing, SAVE setlevel descrs so that you can reattach 
            // them to the set during UNDO
            CSeqdesc_CI desc_ci( m_Set_SEH, 
                                 CSeqdesc::e_not_set, /* all descrs */
                                 1 /* search this seqentry only */ );
            m_SetLevelDescrs.clear();
            for( ; desc_ci; ++desc_ci ) {
                m_SetLevelDescrs.push_back( CConstRef<CSeqdesc>(&*desc_ci) );
            }

            // Renormalize the lone nuc that's inside the nuc-prot set into a 
            // nuc bioseq.  This call will remove annots/descrs from the set and 
            // attach them to the seq.
            m_Set_SEH.GetEditHandle().ConvertSetToSeq();
        }
    }
}

void CCmdDelBioseqInst::Unexecute() 
{
    if (!m_BSH.IsRemoved()) return;
    objects::CBioseq_set_Handle bssh = m_BSH.GetParentBioseq_set();
    if (m_Set_SEH && !m_Set_SEH.IsSet() && m_Inst->IsAa()) {

	// If there were setlevel descrs originally, restore them by
	// removing them from the seq and adding them to the set
        vector< CRef<CSeqdesc> > removedDescrs;
	for (unsigned int bb = 0; bb < m_SetLevelDescrs.size(); ++bb) {
	    // Remove the descr from the seq and store the returned
	    // non-const! CRef<CSeqdesc> for later insertion to nuc-prot set.
	    // Later call to AddSeqdesc requires non-const obj
	    removedDescrs.push_back(
	        m_Set_SEH.GetEditHandle().RemoveSeqdesc(*m_SetLevelDescrs[bb]));
	}

        // UNDO the renormalization
        CSeq_entry_EditHandle edit_handle = m_Set_SEH.GetEditHandle();
        edit_handle.ConvertSeqToSet();
        edit_handle.SetSet().SetClass(CBioseq_set::eClass_nuc_prot);


	// If there were setlevel annots originally, restore them by
	// taking them from the nuc
	for (unsigned int ii = 0; ii < m_SetLevelAnnots.size(); ++ii) {
	    m_Set_SEH.GetEditHandle().TakeAnnot( 
                m_SetLevelAnnots[ii].GetEditHandle() );
	}

	// If there were setlevel descrs originally, restore them by
	// removing them from the seq and adding them to the set
	for (unsigned int nn = 0; nn < removedDescrs.size(); ++nn) {
	    m_Set_SEH.GetEditHandle().AddSeqdesc(*removedDescrs[nn]);
	}
    }

    if (m_Set_SEH &&m_Set_SEH.IsSet())
        bssh = m_Set_SEH.GetSet();

    if (bssh)
    {
        CBioseq_set_EditHandle eh(bssh);
        m_BSH = eh.AttachBioseq(const_cast<CBioseq &>(*m_Inst), m_index); 
    }
    else if (m_Set_SEH) // TODO
    {
        CSeq_entry_EditHandle eh(m_Set_SEH);
        m_BSH = eh.AttachBioseq(const_cast<CBioseq &>(*m_Inst));
    }
}

string CCmdDelBioseqInst::GetLabel()
{
    return "Delete Bioseq";
}


CCmdDelBioseq::CCmdDelBioseq(CBioseq_Handle& bsh)
    : m_BSH(bsh)
{
}

void CCmdDelBioseq::Execute()  
{
    if (m_BSH.IsRemoved()) return;
    objects::CBioseq_set_Handle bssh = m_BSH.GetParentBioseq_set(); // get the parent set - could be empty
    objects::CSeq_entry_Handle seh =  m_BSH.GetSeq_entry_Handle();
    if (bssh) {
        m_index = bssh.GetSeq_entry_Index(seh);   // undocumented function but seems to do what it says
        m_Set_SEH = bssh.GetParentEntry();
    } else {
        m_index = -1;
        m_Set_SEH = seh;
    }

    m_Inst.Reset();
    m_Inst = m_BSH.GetCompleteBioseq();
    CBioseq_EditHandle eh(m_BSH);
    eh.Remove();
}

void CCmdDelBioseq::Unexecute() 
{
    if (!m_BSH.IsRemoved()) return;
    objects::CBioseq_set_Handle bssh = m_BSH.GetParentBioseq_set();

    if (m_Set_SEH &&m_Set_SEH.IsSet())
        bssh = m_Set_SEH.GetSet();

    if (bssh)
    {
        CBioseq_set_EditHandle eh(bssh);
        m_BSH = eh.AttachBioseq(const_cast<CBioseq &>(*m_Inst), m_index); 
    }
    else if (m_Set_SEH) // TODO
    {
        CSeq_entry_EditHandle eh(m_Set_SEH);
        m_BSH = eh.AttachBioseq(const_cast<CBioseq &>(*m_Inst));
    }
}

string CCmdDelBioseq::GetLabel()
{
    return "Delete Bioseq (simple)";
}

END_NCBI_SCOPE
