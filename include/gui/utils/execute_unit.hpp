#ifndef GUI_UTILS___EXECUTE_UNIT__HPP
#define GUI_UTILS___EXECUTE_UNIT__HPP

/*  $Id: execute_unit.hpp 26667 2012-10-19 17:47:39Z katargir $
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

BEGIN_NCBI_SCOPE

class ICanceled;

///////////////////////////////////////////////////////////////////////////////
// IExecuteUnit is interface that allows to execute potentially long code
// on either main or worker thread providing ability to interrupt proccess.
// No exceptions can be thrown from either member

class IExecuteUnit
{
public:
    virtual ~IExecuteUnit() {}

    // This member is called on the main thread to allow
    // some preliminary actions including user interactions
    // for example to allow a user to select options
    // Shouldn't execute lenghty computations
    // Returns true to continue or false to cancel execution
    virtual bool PreExecute() = 0;

    // This member can be called either on the main thread or worker thread
    // implements lengthy computations
    // Returns true on success or false on failure
    virtual bool Execute(ICanceled& canceled) = 0;

    // This member is called on the main thread to implement some postproccessing
    // including user interactions for example to report results, errors or resolve problems
    // Shouldn't execute lenghty computations
    // Returns true to finish execution or false to cancel
    virtual bool PostExecute() = 0;
};

END_NCBI_SCOPE


#endif  // GUI_UTILS___EXECUTE_UNIT__HPP

