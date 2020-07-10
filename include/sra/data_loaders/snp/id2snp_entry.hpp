#ifndef SRA__LOADER__SNP__ID2SNP_ENTRY__HPP
#define SRA__LOADER__SNP__ID2SNP_ENTRY__HPP

/*  $Id: id2snp_entry.hpp 498828 2016-04-19 21:05:09Z vasilche $
* ===========================================================================
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
* ===========================================================================
*
*  Author:  Eugene Vasilchenko
*
*  File Description: Data reader from ID2
*
*/

#include <objects/id2/id2processor_interface.hpp>

BEGIN_NCBI_SCOPE

extern "C" {

NCBI_ID2PROC_SNP_EXPORT
void ID2Processors_Register_SNP(void);

NCBI_ID2PROC_SNP_EXPORT
void NCBI_EntryPoint_id2proc_snp(
     CPluginManager<objects::CID2Processor>::TDriverInfoList&   info_list,
     CPluginManager<objects::CID2Processor>::EEntryPointRequest method);

} // extern C


END_NCBI_SCOPE

#endif//SRA__LOADER__SNP__ID2SNP_ENTRY__HPP
