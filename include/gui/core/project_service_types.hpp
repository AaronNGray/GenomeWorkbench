#ifndef GUI_CORE___PROJECT_SERVICE_EXCEPTION__HPP
#define GUI_CORE___PROJECT_SERVICE_EXCEPTION__HPP

/*  $Id: project_service_types.hpp 43436 2019-06-28 16:26:08Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *    Exception class for the documents
 */


#include <corelib/ncbiexpt.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CProjectServiceException
class CProjectServiceException : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    enum EErrCode {
        eInvalidOperation, /// operation is not permitted in this context
        eInvalidArguments, /// arguments provided to an operation are invalid
        eThreadAffinityError, /// thread affinity is violated
        eLockingError,
        eOperationFailed
    };

    // Translate the specific error code into a string representations of
    // that error code.
    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eInvalidOperation:
            return "eInvalidOperation";

        case eInvalidArguments:
            return "eInvalidArguments";

        case eThreadAffinityError:
            return "eThreadAffinityError";

        case eLockingError:
            return "eLockingError";

        case eOperationFailed:
            return "eOperationFailed";

        default:
            return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CProjectServiceException, CException);
};

END_NCBI_SCOPE


#endif  // GUI_CORE___PROJECT_SERVICE_EXCEPTION__HPP
