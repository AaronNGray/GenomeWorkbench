#ifndef PKG_SEQUENCE___AGP_EXPORT_JOB__HPP
#define PKG_SEQUENCE___AGP_EXPORT_JOB__HPP

/*  $Id: phy_export_job.hpp 28968 2013-09-19 16:11:57Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/utils/app_job_impl.hpp>
#include <gui/objutils/objects.hpp>

#include <gui/packages/pkg_alignment/phy_export_params.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CPhyExportJob
class CPhyExportJob : public CAppJob
{
public:
    CPhyExportJob(const CPhyExportParams& params);

protected:
    virtual EJobState   Run();

    CPhyExportParams m_Params;
};

END_NCBI_SCOPE


#endif // PKG_SEQUENCE___AGP_EXPORT_JOB__HPP
