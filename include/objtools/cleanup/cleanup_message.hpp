#ifndef _CLEANUP_MESSAGE_HPP_
#define _CLEANUP_MESSAGE_HPP_

/*  $Id: cleanup_message.hpp 608332 2020-05-14 16:04:14Z ivanov $
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
 * Author:  Justin Foley
 *
 * File Description:
 *   .......
 *
 */

#include <objtools/logging/message.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class NCBI_CLEANUP_EXPORT CCleanupMessage : public CObjtoolsMessage
{
public:
    enum class ECode {
        eCodeBreak
    };

    enum class ESubcode {
        eBadLocation,
        eParseError
    };

    CCleanupMessage(string text, EDiagSev sev, ECode code, ESubcode subcode);

    CCleanupMessage *Clone(void) const override;

    int GetCode(void) const override {
        return static_cast<int>(m_Code);
    }
    int GetSubCode(void) const override {
        return static_cast<int>(m_Subcode);
    }
private:
    ECode m_Code;
    ESubcode m_Subcode;
};

END_SCOPE(objects)
END_NCBI_SCOPE    

#endif // _CLEANUP_MESSAGE_HPP_
