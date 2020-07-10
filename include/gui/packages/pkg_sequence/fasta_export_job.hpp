#ifndef PKG_SEQUENCE___FASTA_EXPORT_JOB__HPP
#define PKG_SEQUENCE___FASTA_EXPORT_JOB__HPP

/*  $Id: fasta_export_job.hpp 38492 2017-05-16 20:11:48Z evgeniev $
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

#include <gui/packages/pkg_sequence/fasta_export_params.hpp>

#include <vector>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CSeq_loc;
class CScope;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// CFastaExportJob
class CFastaExportJob : public CAppJob
{
public:
    CFastaExportJob(const CFastaExportParams& params);

protected:
    virtual EJobState   Run();

    typedef std::vector<CConstRef<objects::CSeq_loc>> TLocVector;

    bool x_CheckForDuplicates(const objects::CSeq_loc &loc, TLocVector &loc_vector, objects::CScope& scope) const;
    CConstRef<objects::CSeq_loc> x_GetWholeLoc(const objects::CSeq_loc &loc, TLocVector &loc_vector, objects::CScope& scope) const;

protected:
    CFastaExportParams m_Params;
};


END_NCBI_SCOPE


#endif // PKG_SEQUENCE___FASTA_EXPORT_JOB__HPP
