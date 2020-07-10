#ifndef PKG_ALIGNMENT___BLAST_SEQ_TOOL_JOB__HPP
#define PKG_ALIGNMENT___BLAST_SEQ_TOOL_JOB__HPP

/*  $Id: blast_seq_tool_job.hpp 33768 2015-09-15 19:42:54Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>

#include <gui/core/loading_app_job.hpp>

struct SBlastProgress;

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CBLASTSeqToolJob
class  CBLASTSeqToolJob : public CDataLoadingAppJob
{
public:
    CBLASTSeqToolJob( const CBLASTParams& params );

    static Boolean s_BlastInterrupt( SBlastProgress* prog );

    virtual void RequestCancel();


protected:
    virtual void x_CreateProjectItems(); // overriding virtual function

    void x_CreateProjectItems( vector< CRef<objects::CSeq_align_set> >& aligns );

protected:
    CBLASTParams m_Params;

private:
    TProcessHandle m_ProcHandle;
};


/* @} */

END_NCBI_SCOPE


#endif  // PKG_ALIGNMENT___BLAST_SEQ_TOOL_JOB__HPP

