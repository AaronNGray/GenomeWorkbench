#ifndef PKG_ALIGNMENT___MUSCLE_TOOL_JOB__HPP
#define PKG_ALIGNMENT___MUSCLE_TOOL_JOB__HPP

/*  $Id: muscle_tool_job.hpp 38490 2017-05-16 18:21:11Z evgeniev $
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
* Authors:  Roman Katargin, Vladislav Evgeniev
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_alignment/msa_tool_job.hpp>
#include <gui/packages/pkg_alignment/muscle_tool_params.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

///////////////////////////////////////////////////////////////////////////////
/// CMuscleToolJob
class  CMuscleToolJob : public CMSAToolJob
{
public:
    CMuscleToolJob(const CMuscleToolParams& params);

protected:
    virtual wxString x_GetCommandLine(const wxString &input, const wxString &output, bool is_nucleotide);
    virtual TConstScopedObjects& x_GetObjects()
    {
        return m_Params.SetObjects();
    }

private:
    CMuscleToolParams   m_Params;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___MUSCLE_TOOL_JOB__HPP
