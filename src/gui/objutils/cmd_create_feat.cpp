/*  $Id: cmd_create_feat.cpp 42389 2019-02-13 19:05:46Z asztalos $
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

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>

#include <gui/objutils/cmd_create_feat.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdCreateFeat::Execute()
{
    if (m_Feat->IsSetData() && m_Feat->GetData().IsCdregion() && m_Feat->IsSetProduct()) {
        CBioseq_Handle bsh;
        try {
            bsh = m_seh.GetScope().GetBioseqHandle(m_Feat->GetLocation());
        }
        catch (const CException&) {
            bsh = m_seh.GetScope().GetBioseqHandle(m_Feat->GetProduct());
        }
        if (bsh) {
            CBioseq_set_Handle nuc_parent = bsh.GetParentBioseq_set();
            if (nuc_parent && nuc_parent.IsSetClass() && nuc_parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                m_seh = nuc_parent.GetParentEntry();
            }
        }
    }

    CSeq_annot_Handle ftable;
    CSeq_annot_CI annot_ci(m_seh, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if ((*annot_ci).IsFtable()) {
            ftable = *annot_ci;
            break;
        }
    }

    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();

    if (!ftable) {
        CRef<CSeq_annot> new_annot(new CSeq_annot());
        ftable = m_FTableCreated = eh.AttachAnnot(*new_annot);
    }

    CSeq_annot_EditHandle aeh(ftable);
    m_feh = aeh.AddFeat(*m_Feat);
}

void CCmdCreateFeat::Unexecute()
{
    m_feh.Remove();

    if (m_FTableCreated) {
        m_FTableCreated.Remove();
    }
}

string CCmdCreateFeat::GetLabel()
{
    return "Create Feature";
}

CIRef<IEditCommand> CCmdCreateFeatBioseq::x_CreateActionCmd()
{
    CSeq_entry_Handle seh = m_bsh.GetSeq_entry_Handle();
    return CIRef<IEditCommand>(new CCmdCreateFeat(seh, *m_Feat));
}

string CCmdCreateFeatBioseq::GetLabel()
{
    return "Create Feature";
}

END_NCBI_SCOPE
