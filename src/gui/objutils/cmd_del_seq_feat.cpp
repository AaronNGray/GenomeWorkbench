/*  $Id: cmd_del_seq_feat.cpp 31627 2014-10-28 21:14:50Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <objmgr/bio_object_id.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdDelSeq_feat::Execute()
{
    if (m_Orig_feat) 
    {
        if (m_Orig_feat.IsRemoved())
            return;
        m_Saved_feat.Reset(new CSeq_feat());
        m_Saved_feat->Assign(*m_Orig_feat.GetOriginalSeq_feat());
        CSeq_feat_EditHandle feh(m_Orig_feat);
        m_aeh = feh.GetAnnot();
        m_Parent_handle =  m_aeh.GetParentEntry();       
        m_GrandParent_handle =  m_Parent_handle.GetParentEntry();       
        feh.Remove();
        const list< CRef< CSeq_feat > > &feat_list = m_aeh.GetSeq_annotCore()->GetData().GetFtable();
        if (feat_list.empty())
        {
            m_aeh.Remove();       
        }
        
    }
}

void CCmdDelSeq_feat::Unexecute()
{
    if (!m_Orig_feat.IsRemoved())
        return;
    
    if (m_Saved_feat) 
    {
        if (!m_aeh.IsRemoved())
        {          
            m_Orig_feat = m_aeh.AddFeat(*m_Saved_feat);            
        }
        else 
        {            
            if (!m_Parent_handle)     // When there was a delete bioseq command in between which performed a conversion of nuc-prot set to seq and back the entry handle is missing sometimes for an unknown reason
            {
                CBioseq_CI seq_ci(m_GrandParent_handle);
                while (seq_ci)
                {
                    if (seq_ci->IsNa())
                    {
                        m_Parent_handle = seq_ci->GetSeq_entry_Handle().GetEditHandle();
                        break;
                    }
                    ++seq_ci;
                }
            }
            
            CSeq_annot_EditHandle aeh = m_Parent_handle.AttachAnnot(m_aeh);
            m_Orig_feat = aeh.AddFeat(*m_Saved_feat);
        }
        
        m_Saved_feat.Reset();
    }
}

string CCmdDelSeq_feat::GetLabel()
{
    return "Delete feature";
}


END_NCBI_SCOPE
