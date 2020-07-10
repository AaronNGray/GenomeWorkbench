#ifndef PKG_ALIGNMENT___ALIGN_TAB_EXPORT_JOB__HPP
#define PKG_ALIGNMENT___ALIGN_TAB_EXPORT_JOB__HPP

/*  $Id: fasta_align_export_job.hpp 38520 2017-05-22 17:33:12Z evgeniev $
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
* Authors: Vladislav Evgeniev
*
* File Description:
*
*/

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <gui/packages/pkg_alignment/fasta_align_export_params.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CFastaAlignExportJob
class CFastaAlignExportJob : public CAppJob
{
public:
    CFastaAlignExportJob(const CFastaAlignExportParams& params);

protected:
    virtual EJobState   Run();

    CFastaAlignExportParams m_Params;
};


END_NCBI_SCOPE


#endif // PKG_ALIGNMENT___ALIGN_TAB_EXPORT_JOB__HPP
