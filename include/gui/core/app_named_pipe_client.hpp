#ifndef GUI_CORE___APP_NAMED_PIPE_CLIENT__HPP
#define GUI_CORE___APP_NAMED_PIPE_CLIENT__HPP

/*  $Id: app_named_pipe_client.hpp 33339 2015-07-08 19:49:33Z katargir $
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

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// IAppNamedPipeClient is an interface that must be implemented
/// by extension that is to be registered at AppNamedPipe extension point
///

class  IAppNamedPipeClient
{
public:
    virtual ~IAppNamedPipeClient() {}

    /// Extension can process data received via named pipe.
    /// If successfully processed and no farther processing needed,
    /// returns true, otherwise - false.
    virtual bool Process(const string& data, string& reply, IWorkbench* workbench) = 0;
    /// Returns true if the extension is the default one.
    /// The default extension will process data received
    /// if all other registered extensions don't process data
    virtual bool IsDefault() const { return false; }   
};

#define EXT_POINT__GBENCH_NAMED_PIPE_CLIENTS "gbench_named_pipe_clients"

END_NCBI_SCOPE

#endif  // GUI_CORE___APP_NAMED_PIPE_CLIENT__HPP
