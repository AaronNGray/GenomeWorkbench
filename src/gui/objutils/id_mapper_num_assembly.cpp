/*  $Id: id_mapper_num_assembly.cpp 42483 2019-03-08 15:24:46Z evgeniev $
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
 * Author:  Roman Katargin
 *
 * File Description: id remapping class
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/objutils/gencoll_svc.hpp>

#include <gui/objutils/id_mapper_num_assembly.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CIdMapperNumAssembly::CIdMapperNumAssembly  (const string &mappingAssembly, IGencollSvcErrors *svcErrors)
{
    CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(mappingAssembly, true, "Gbench", svcErrors);
    if (assm) {
        CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
        scope->AddDefaults();
        CIdMapperGCAssembly::EAliasMapping alias = assm->IsRefSeq() ? CIdMapperGCAssembly::eRefSeqAcc : CIdMapperGCAssembly::eGenBankAcc;
        m_GCAssemblyMapper.reset(new CIdMapperGCAssembly(*scope, *assm, alias));
    }

    m_NumMapper.reset(new CIdMapper());

    x_InitializeMapping();
}


CIdMapperNumAssembly::CIdMapperNumAssembly(CRef<CGC_Assembly> assm)
{
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();
    CIdMapperGCAssembly::EAliasMapping alias = assm->IsRefSeq() ? CIdMapperGCAssembly::eRefSeqAcc : CIdMapperGCAssembly::eGenBankAcc;
    m_GCAssemblyMapper.reset(new CIdMapperGCAssembly(*scope, *assm, alias));

    m_NumMapper.reset(new CIdMapper());

    x_InitializeMapping();
}


CSeq_id_Handle CIdMapperNumAssembly::Map(const CSeq_id_Handle& id)
{
    CSeq_id_Handle h = m_NumMapper->Map(id);
    if (!m_GCAssemblyMapper)
        return h;
    return m_GCAssemblyMapper->Map(h ? h : id);
}

CRef<CSeq_loc> CIdMapperNumAssembly::Map(const CSeq_loc& loc)
{
    CRef<CSeq_loc> ret = m_NumMapper->Map(loc);
    if (!m_GCAssemblyMapper)
        return ret;
    return m_GCAssemblyMapper->Map(ret ? *ret : loc);
}

void CIdMapperNumAssembly::x_InitializeMapping()
{
    for (int i = 1; i <= 22; ++i) {
        string strId = NStr::NumericToString(i);
        CSeq_id source(CSeq_id::e_Local, strId);
        CSeq_id target(CSeq_id::e_Local, "chr" + strId);
        m_NumMapper->AddMapping(CSeq_id_Handle::GetHandle(source), CSeq_id_Handle::GetHandle(target));
    }

    {
        CSeq_id source(CSeq_id::e_Local, "X");
        CSeq_id target(CSeq_id::e_Local, "chrX");
        m_NumMapper->AddMapping(CSeq_id_Handle::GetHandle(source), CSeq_id_Handle::GetHandle(target));
    }

    {
        CSeq_id source(CSeq_id::e_Local, "Y");
        CSeq_id target(CSeq_id::e_Local, "chrY");
        m_NumMapper->AddMapping(CSeq_id_Handle::GetHandle(source), CSeq_id_Handle::GetHandle(target));
    }

    {
        CSeq_id source(CSeq_id::e_Local, "M");
        CSeq_id target(CSeq_id::e_Local, "chrM");
        m_NumMapper->AddMapping(CSeq_id_Handle::GetHandle(source), CSeq_id_Handle::GetHandle(target));
    }

    {
        CSeq_id source(CSeq_id::e_Local, "MT");
        CSeq_id target(CSeq_id::e_Local, "chrM");
        m_NumMapper->AddMapping(CSeq_id_Handle::GetHandle(source), CSeq_id_Handle::GetHandle(target));
    }
}

END_NCBI_SCOPE
