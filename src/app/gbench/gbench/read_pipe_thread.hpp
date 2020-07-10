#ifndef GUI_GBENCH_NEW___READ_PIPE_THREAD__HPP
#define GUI_GBENCH_NEW___READ_PIPE_THREAD__HPP

/*  $Id: read_pipe_thread.hpp 33382 2015-07-15 19:01:26Z katargir $
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
#include <corelib/ncbithr.hpp>

BEGIN_NCBI_SCOPE

class CNamedPipeServer;

class CReadPipeThread : public CThread
{
public:
    CReadPipeThread(CNamedPipeServer& pipe, IWorkbench* workbench);
    void RequestStop();

protected:
    /// Do job delegated processing to the main class
    virtual void* Main(void);

    CNamedPipeServer&  m_Pipe;
    IWorkbench*        m_Workbench;
    mutable CSemaphore m_StopSignal;
};

END_NCBI_SCOPE


#endif  // GUI_GBENCH_NEW___READ_PIPE_THREAD__HPP
