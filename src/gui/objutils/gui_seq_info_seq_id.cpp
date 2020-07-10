/*  $Id: gui_seq_info_seq_id.cpp 26341 2012-08-29 20:54:24Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_seq_info.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/general/Dbtag.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiSeqInfoSeq_id : public CObject, public IGuiSeqInfo
{
public:
    static CGuiSeqInfoSeq_id* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual bool IsSequence() const { return true; }
    virtual bool IsDNA() const;
    virtual bool IsProtein() const;
    virtual bool IsGenomic() const;
    virtual bool IsRNA() const { return false; }
    virtual bool IscDNA() const { return false; }

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
};

void initCGuiSeqInfoSeq_id()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiSeqInfo).name(),
            CSeq_id::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiSeqInfoSeq_id>());
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiSeqInfo).name(),
            CSeq_loc::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiSeqInfoSeq_id>());
}

CGuiSeqInfoSeq_id* CGuiSeqInfoSeq_id::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    const CObject* obj_ptr = object.object.GetPointer();
    const CSeq_loc* seq_loc_ptr = dynamic_cast<const CSeq_loc*>(obj_ptr);
    if (seq_loc_ptr) {
        obj_ptr = seq_loc_ptr->GetId();
    }
    if (obj_ptr == 0)
        return 0;

    CGuiSeqInfoSeq_id* gui_info = new CGuiSeqInfoSeq_id();
    gui_info->m_Object = obj_ptr;
    gui_info->m_Scope  = object.scope;

    return gui_info;
}

bool CGuiSeqInfoSeq_id::IsGenomic() const
{
    return !IsProtein();
}

bool CGuiSeqInfoSeq_id::IsProtein() const
{
    const CSeq_id* seq_id_ptr = dynamic_cast<const CSeq_id*>(m_Object.GetPointer());
    if (seq_id_ptr->IsGi())
        seq_id_ptr = sequence::GetId(*seq_id_ptr, *m_Scope, sequence::eGetId_Best).GetSeqId();
    CSeq_id::EAccessionInfo info = seq_id_ptr->IdentifyAccession();
    return (info&CSeq_id::fAcc_prot) != 0 ? true : false;
}

bool CGuiSeqInfoSeq_id::IsDNA() const
{
    const CSeq_id* seq_id_ptr = dynamic_cast<const CSeq_id*>(m_Object.GetPointer());
    if (seq_id_ptr->IsGi())
        seq_id_ptr = sequence::GetId(*seq_id_ptr, *m_Scope, sequence::eGetId_Best).GetSeqId();

    CSeq_id::EAccessionInfo info = seq_id_ptr->IdentifyAccession();

    if (info == CSeq_id::eAcc_general) {
        const CSeq_id::TGeneral& gen = seq_id_ptr->GetGeneral();
        if (gen.GetDb() == "TRACE_ASSM") {
            return true;
        }
    }

    if(info & CSeq_id::fAcc_prot)
        return false;

    return true;
}

END_NCBI_SCOPE
