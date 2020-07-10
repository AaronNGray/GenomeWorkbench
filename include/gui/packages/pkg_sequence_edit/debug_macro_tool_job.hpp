#ifndef PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_JOB__HPP
#define PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_JOB__HPP

/*  $Id: debug_macro_tool_job.hpp 30812 2014-07-25 18:57:09Z katargir $
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
* Authors:  Roman Katargin
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>

#include <gui/core/loading_app_job.hpp>
#include <gui/packages/pkg_sequence_edit/debug_macro_tool_params.hpp>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_loc;
END_SCOPE(objects)

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

///////////////////////////////////////////////////////////////////////////////
/// CDebugMacroToolJob
class  CDebugMacroToolJob : public CDataLoadingAppJob
{
public:
    CDebugMacroToolJob(const CDebugMacroToolParams& params);

protected:
    virtual void    x_CreateProjectItems();

private:
    CDebugMacroToolParams m_Params;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_JOB__HPP
