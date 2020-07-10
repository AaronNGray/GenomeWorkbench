/*  $Id: gui_object_info_seq_entry.cpp 30817 2014-07-25 20:42:39Z katargir $
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
#include <gui/objutils/gui_object_info.hpp>

#include <objects/seqset/Seq_entry.hpp>

#include "gui_object_info_bioseq.hpp"
#include "gui_object_info_bioseq_set.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoSeq_entry : public CObject, public IGuiObjectInfo
{
public:
    static CObject* CreateObject(SConstScopedObject& object, ICreateParams* params);
};

void initCGuiObjectInfoSeq_entry()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_entry::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_entry>());
}

CObject* CGuiObjectInfoSeq_entry::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*>(object.object.GetPointerOrNull());
    if (seqEntry) {
        if (seqEntry->IsSeq()) {
            CGuiObjectInfoBioseq* gui_info = new CGuiObjectInfoBioseq();
            gui_info->m_Object = CConstRef<CObject>(&seqEntry->GetSeq());
            gui_info->m_Scope  = object.scope;
            return gui_info;
        } else if (seqEntry->IsSet()) {
            CGuiObjectInfoBioseq_set* gui_info = new CGuiObjectInfoBioseq_set();
            gui_info->m_Object = CConstRef<CObject>(&seqEntry->GetSet());
            gui_info->m_Scope  = object.scope;
            return gui_info;
        }
    }
    return 0;
}

END_NCBI_SCOPE
