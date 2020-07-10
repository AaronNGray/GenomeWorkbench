/*  $Id: cmd_change_bioseq_inst.cpp 27501 2013-02-26 17:41:50Z bollin $
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
#include <gui/objutils/cmd_change_bioseq_inst.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdChangeBioseqInst::CCmdChangeBioseqInst(const CBioseq_Handle& bh, const CSeq_inst& new_inst)
    : m_BH(bh)
{ 
    m_Inst.Reset(new CSeq_inst());
    m_Inst->Assign(new_inst);
}

void CCmdChangeBioseqInst::Execute()
{
    CRef<CSeq_inst> saveInst(new CSeq_inst());
    if (m_BH.CanGetInst()) {
        saveInst->Assign(m_BH.GetInst());
    }

    CBioseq_EditHandle eh = m_BH.GetEditHandle();
    eh.SetInst(*m_Inst);
    m_Inst = saveInst;
}

void CCmdChangeBioseqInst::Unexecute()
{
    Execute();
}

string CCmdChangeBioseqInst::GetLabel()
{
    return "Modify Bioseq Inst";
}

END_NCBI_SCOPE
