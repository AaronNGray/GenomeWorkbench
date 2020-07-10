#ifndef GUI_OBJUTILS___ID_MAPPER_NUM_ASSEMBLY_HPP
#define GUI_OBJUTILS___ID_MAPPER_NUM_ASSEMBLY_HPP

/*  $Id: id_mapper_num_assembly.hpp 42483 2019-03-08 15:24:46Z evgeniev $
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

#include <algo/id_mapper/id_mapper.hpp>
#include <objtools/readers/idmapper.hpp>
#include <gui/objutils/gencoll_svc.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CIdMapperNumAssembly : public objects::CIdMapper
{
public:
    CIdMapperNumAssembly  (const string &mappingAcc, IGencollSvcErrors *svcErrors = nullptr);
    CIdMapperNumAssembly(CRef<objects::CGC_Assembly> assm);

    virtual objects::CSeq_id_Handle Map(const objects::CSeq_id_Handle& id);
    virtual CRef<objects::CSeq_loc> Map(const objects::CSeq_loc& loc);

    bool IsAssemblyMapper() const { return m_GCAssemblyMapper ? true : false; }

private:
    void x_InitializeMapping();

    unique_ptr<objects::CIdMapper> m_NumMapper;
    unique_ptr<objects::CIdMapperGCAssembly> m_GCAssemblyMapper;
};

END_NCBI_SCOPE

#endif // GUI_OBJUTILS___ID_MAPPER_NUM_ASSEMBLY_HPP
