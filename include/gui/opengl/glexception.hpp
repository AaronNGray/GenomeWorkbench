#ifndef GUI_OPENGL___GLEXCEPTION__HPP
#define GUI_OPENGL___GLEXCEPTION__HPP

/*  $Id: glexception.hpp 43434 2019-06-28 16:07:02Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiexpt.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// class COpenGLException defines exceptions thrown from the OpenGL subsystem.
//

class COpenGLException : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    // Enumerated list of document management errors
    enum EErrCode {
        eTextureError,
        eGenericError,
        eGlError
    };

    // Translate the specific error code into a string representations of
    // that error code.
    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eTextureError: return "eTextureError";
        case eGenericError: return "eGenericError";
        case eGlError:      return "eGlError";
        default:                return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(COpenGLException, CException);
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLEXCEPTION__HPP
