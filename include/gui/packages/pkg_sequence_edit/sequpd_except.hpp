#ifndef _GUI_PACKAGES_SEQUPD_EXCEPT_HPP_
#define _GUI_PACKAGES_SEQUPD_EXCEPT_HPP_

/*  $Id: sequpd_except.hpp 40492 2018-02-27 21:12:15Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiexpt.hpp>

BEGIN_NCBI_SCOPE

/// Sequence update exception class
class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CSeqUpdateException : public CException
{
public:
    enum EErrCode {
        eInternal,              // Internal error (usually indicates attempt to derefence non-initialized pointer)
        eReading,               // Error encountered during reading the update sequence
        eAlignment,             // Error encountered during forming the alignment
        eFeatAdjust,            // Error encountered during adjusting existing features on the sequence
        eFeatImport             // Error encountered during importing new features from the update sequence
    };

    virtual const char* GetErrCodeString(void) const
    {
        switch (GetErrCode())
        {
        case eInternal:   return "eInternal";
        case eReading:    return "eReading";
        case eAlignment:  return "eAlignment";
        case eFeatAdjust: return "eFeatAdjust";
        case eFeatImport: return "eFeatImport";
        default: 
            return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CSeqUpdateException, CException);
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_SEQUPD_EXCEPT_HPP_
