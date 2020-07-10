/*  $Id: fasta_align_export_job.cpp 38593 2017-06-01 17:31:50Z evgeniev $
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
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>
#include <gui/packages/pkg_alignment/fasta_align_export_job.hpp>
#include <gui/objutils/utils.hpp>
#include <objtools/writers/aln_writer.hpp>
#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CFastaAlignExportJob::CFastaAlignExportJob(const CFastaAlignExportParams& params)
: CAppJob("FASTA Alignment Export"), m_Params(params)
{
}



IAppJob::EJobState CFastaAlignExportJob::Run()
{
    try {
        SConstScopedObject seq_align = m_Params.GetObject();
        const CSeq_align* align = dynamic_cast<const CSeq_align*> (seq_align.object.GetPointer());
        CScope& scope = seq_align.scope.GetObject();

        CNcbiOfstream ostr(m_Params.GetFileName().fn_str());
        CAlnWriter writer(scope, ostr, CWriterBase::fNormal);
        writer.WriteAlign(*align);
	}
    catch (const CException& e) {
        string err_msg;
        err_msg = "Failed to export FASTA alignment:\n";
        err_msg += e.GetMsg();
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }

    LOG_POST(Info << "CFastaAlignExportJob::Run()  Finished  " << m_Descr);
    return eCompleted;
 }

END_NCBI_SCOPE
